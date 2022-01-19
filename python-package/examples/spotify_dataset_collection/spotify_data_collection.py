import os
import time
import argparse
import threading

import pandas as pd
from brainflow.board_shim import BoardShim, BrainFlowInputParams, LogLevels, BoardIds
from brainflow.data_filter import DataFilter
import spotipy
import spotipy.util as util


class DataThread(threading.Thread):

    def __init__(self, board, spotify):
        threading.Thread.__init__ (self)
        self.keep_alive = True
        self.board = board
        self.spotify = spotify

    def run(self):
        current_song_id = None
        prev_song_id = None
        is_playing = False
        is_end = False
        # handle the case of skipping songs
        needed_duration = 10  # in seconds
        time_sleep = 1
        counter_for_duration = 0
        counter_max = int(needed_duration/time_sleep)
        current_time = str(time.time())
        folder_name = str(self.board.get_board_id())
        if not os.path.exists(folder_name):
            os.makedirs(folder_name)
        brainflow_output_file = os.path.join(folder_name, 'brainflow' + current_time + '.csv')
        song_features_output_file = os.path.join(folder_name, 'songs' + current_time + '.csv')

        while self.keep_alive:
            time.sleep(time_sleep)
            track = self.spotify.current_user_playing_track()
            if track is not None:
                # despite the check above track obj can become None in case of ads
                try:
                    if track.get('item', {}).get('id') != current_song_id and current_song_id is not None:
                        is_end = True
                    if track.get('is_playing', False) and not is_end:
                        try:
                            # despite the check above track object can become None, no idea how
                            current_song_id = track.get('item', {}).get('id')
                            counter_for_duration = counter_for_duration + 1
                            is_end = False
                            is_playing = True
                        except BaseException:
                            pass
                    elif not track.get('is_playing', True):
                        is_end = True
                except AttributeError as e:
                    BoardShim.log_message(LogLevels.LEVEL_WARN.value, 'Exception occured, more likely because of ads(its ok): %s' % str(e))
            else:
                is_end = True

            if is_end:
                prev_song_id = current_song_id
                current_song_id = None
                data = self.board.get_board_data()
                # store data when a song ends
                if is_playing and counter_for_duration >= counter_max and prev_song_id is not None and data.shape[1] > 1:
                    DataFilter.write_file(data, brainflow_output_file, 'a')
                    features = self.spotify.audio_features(prev_song_id)
                    BoardShim.log_message(LogLevels.LEVEL_DEBUG.value, 'adding info about song: %s' % prev_song_id)
                    features_df = pd.DataFrame.from_dict(features)
                    music_feature = features_df[['danceability', 'energy', 'loudness', 'speechiness',
                        'acousticness', 'instrumentalness', 'liveness', 'valence', 'tempo', 'id']]
                    music_features_replicated = pd.concat([music_feature] * (data.shape[1] - 1), ignore_index=True)
                    music_features_replicated.to_csv(song_features_output_file, sep='\t', mode='a')
                is_playing = False
                counter_for_duration = 0
                is_end = False


def get_input():
    parser = argparse.ArgumentParser()
    parser.add_argument('--runtime', type=int, help='max time to sleep in the main thread in seconds',
        default=1800, required=False)
    # brainflow args
    parser.add_argument('--timeout', type=int, help='timeout for device discovery or connection', required=False,
                        default=0)
    parser.add_argument('--ip-port', type=int, help='ip port', required=False, default=0)
    parser.add_argument('--ip-protocol', type=int, help='ip protocol, check IpProtocolType enum', required=False,
                        default=0)
    parser.add_argument('--ip-address', type=str, help='ip address', required=False, default='')
    parser.add_argument('--serial-port', type=str, help='serial port', required=False, default='')
    parser.add_argument('--mac-address', type=str, help='mac address', required=False, default='')
    parser.add_argument('--other-info', type=str, help='other info', required=False, default='')
    parser.add_argument('--streamer-params', type=str, help='streamer params', required=False, default='')
    parser.add_argument('--serial-number', type=str, help='serial number', required=False, default='')
    parser.add_argument('--board-id', type=int, help='board id, check docs to get a list of supported boards',
                        required=False, default=int(BoardIds.ENOPHONE_BOARD))
    parser.add_argument('--file', type=str, help='file', required=False, default='')
    # spotify args
    parser.add_argument('--redirect-uri', type=str, default=None)
    parser.add_argument('--client-id', type=str, default=None)
    parser.add_argument('--client-secret', type=str, default=None)
    parser.add_argument('--username', type=str, required=True)
    args = parser.parse_args()

    params = BrainFlowInputParams()
    params.ip_port = args.ip_port
    params.serial_port = args.serial_port
    params.mac_address = args.mac_address
    params.other_info = args.other_info
    params.serial_number = args.serial_number
    params.ip_address = args.ip_address
    params.ip_protocol = args.ip_protocol
    params.timeout = args.timeout
    params.file = args.file

    token = util.prompt_for_user_token(args.username,
                                       scope='user-read-currently-playing',
                                       client_id=args.client_id,
                                       client_secret=args.client_secret,
                                       redirect_uri=args.redirect_uri)

    return args.board_id, params, token, args.runtime

def main():
    BoardShim.enable_dev_board_logger()
    board_id, params, token, runtime = get_input()
    board = BoardShim(board_id, params)
    sp = spotipy.Spotify(auth=token)
    data_thread = DataThread(board, sp)
    try:
        data_thread.start()
        board.prepare_session()
        board.start_stream()
        time.sleep(runtime)
    except BaseException:
        pass
    finally:
        data_thread.keep_alive = False
        data_thread.join()
        if board.is_prepared():
            board.release_session()


if __name__ == "__main__":
    main ()
