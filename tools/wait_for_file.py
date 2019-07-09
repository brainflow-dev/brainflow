import argparse
import logging
import time

import dropbox


def check_exists (path, token):
    dbx = dropbox.Dropbox (token)
    # wait for 20 minutes, should be enough
    for i in range (20):
        try:
            dbx.files_get_metadata (path)
            return True
        except:
            logging.info ('no %s found, atempts %d' % (path, i))
            time.sleep (60)
    return False


def main ():
    logging.basicConfig (level = logging.INFO, format = '%(asctime)s [%(levelname)s] %(message)s')

    parser = argparse.ArgumentParser ()
    parser.add_argument ('--token', type = str, help  = 'access token', required = True)
    parser.add_argument ('--file', type = str, help = 'remote file location', required = True)
    args = parser.parse_args ()

    check_exists (args.file, args.token)


if __name__ == '__main__':
    main ()
