import argparse
import os
import logging

import dropbox


class TransferData:

    def __init__ (self, access_token):
        self.access_token = access_token

    def list_files (self, path):
        result = list ()
        dbx = dropbox.Dropbox (self.access_token)
        response = dbx.files_list_folder (path = path)
        for entry in response.entries:
            result.append ((entry.name, entry.path_display))
        return result


    def download_file (self, file_from, file_to):
        logging.info ('downloading %s to %s' % (file_from, file_to))
        dbx = dropbox.Dropbox (self.access_token)
        dbx.files_download_to_file (file_to, file_from)


def main ():
    logging.basicConfig (level = logging.INFO, format = '%(asctime)s [%(levelname)s] %(message)s')

    parser = argparse.ArgumentParser ()
    parser.add_argument ('--token', type = str, help  = 'access token', required = True)
    parser.add_argument ('--remote-dir', type = str, help  = 'remote dir to download files from', required = True)
    parser.add_argument ('--local-dir', type = str, help = 'local dir', required = True)
    args = parser.parse_args ()

    transfer_data = TransferData (args.token)
    files = transfer_data.list_files (args.remote_dir)
    for file in files:
        transfer_data.download_file (file[1], os.path.join (args.local_dir, file[0]))


if __name__ == '__main__':
    main ()
