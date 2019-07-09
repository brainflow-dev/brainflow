import argparse
import os
import logging

import dropbox
import glob


class TransferData:

    def __init__ (self, access_token):
        self.access_token = access_token

    def check_exists (self, path):
        dbx = dropbox.Dropbox (self.access_token)
        try:
            dbx.files_get_metadata (path)
            return True
        except:
            return False

    def upload_file (self, file_from, file_to):
        if self.check_exists (file_to):
            logging.warning ('file %s already exists' % file_to)
        else:
            logging.info ('sending %s to %s' % (file_from, file_to))
            dbx = dropbox.Dropbox (self.access_token)
            with open (file_from, 'rb') as f:
                dbx.files_upload (f.read (), file_to)


def main ():
    logging.basicConfig (level = logging.INFO, format = '%(asctime)s [%(levelname)s] %(message)s')

    parser = argparse.ArgumentParser ()
    parser.add_argument ('--token', type = str, help  = 'access token', required = True)
    parser.add_argument ('--local-files', type = str, help  = 'local file to send', required = True)
    parser.add_argument ('--remote-dir', type = str, help = 'remote file location', required = True)
    args = parser.parse_args ()

    transfer_data = TransferData (args.token)
    logging.info ('remote dir %s' % args.remote_dir)
    logging.info ('files: %s' % '\n'.join(glob.glob (args.local_files)))
    for file in glob.glob (args.local_files):
        remote_file = args.remote_dir + '/' + os.path.split (file)[1]
        try:
            transfer_data.upload_file (file, remote_file)
        except Exception as e:
            logging.warning (str (e))


if __name__ == '__main__':
    main ()
