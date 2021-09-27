import threading
import time
import json
import socket
import subprocess
import sys
import logging
from random import randint
from http.server import HTTPServer, BaseHTTPRequestHandler

from brainflow_emulator.emulate_common import log_multilines, TestFailureError


class ShieldWriter(threading.Thread):

    def __init__(self, write_socket):
        threading.Thread.__init__(self)
        self.write_socket = write_socket
        self.delay = 1
        self.package_size = 33
        self.package_num = 0
        self.need_data = True

    def run(self):
        while self.need_data:
            package = list()
            for _ in range(6):
                package.append(0xA0)
                package.append(self.package_num)
                for i in range(2, self.package_size - 1):
                    package.append(randint(0, 255))
                package.append(0xC0)
                self.package_num = self.package_num + 1
                if self.package_num % 256 == 0:
                    self.package_num = 0
            self.write_socket.sendall(bytes(package))
            time.sleep(self.delay)


send_sock = None
write_thread = None
ip = None
port = None


class Handler(BaseHTTPRequestHandler):

    def _set_headers(self):
        self.send_response(200)
        self.send_header("Content-type", "text/html")
        self.end_headers()

    def _html(self, message):
        content = "<html><body><h1>%s</h1></body></html>" % message
        return content.encode("utf8")

    def do_GET(self):
        global send_sock
        global write_thread
        global ip
        global port

        if (self.path == '/stream/start'):
            logging.info('ip %s port %s' % (str(ip), str(port)))
            if send_sock is None:
                raise TestFailureError('socket was not created', -1)
            else:
                write_thread = ShieldWriter(send_sock)
                write_thread.daemon = True
                write_thread.start()
        elif (self.path == '/stream/stop'):
            if write_thread is None:
                raise TestFailureError('send thread was not created', -1)
            else:
                write_thread.need_data = False
                write_thread.join()

        self._set_headers()
        self.wfile.write(self._html("get was called"))

    def do_HEAD(self):
        self._set_headers()

    def do_POST(self):
        global send_sock
        global write_thread
        global ip
        global port

        if self.path == '/tcp':
            logging.info('creating tcp socket')
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length)
            received_data = post_data.decode("utf-8")
            json_data = json.loads(received_data)
            port = int(json_data.get('port'))
            ip = str(json_data.get('ip'))
            server_addr = (ip, port)
            send_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            send_sock.connect(server_addr)
            logging.info('socket was created')

        self._set_headers()
        self.wfile.write(self._html("post was called"))


class ThreadedHTTPServer(object):

    def __init__(self):
        server_address = ('127.0.0.1', 80)
        server_class = HTTPServer
        handler_class = Handler
        self.httpd = server_class(server_address, handler_class)
        self.server_thread = threading.Thread(target=self.httpd.serve_forever)
        self.server_thread.daemon = True

    def start(self):
        self.server_thread.start()

    def stop(self):
        self.httpd.shutdown()
        self.httpd.server_close()


def test_shield(cmd_list):
    server = ThreadedHTTPServer()
    server.start()

    logging.info('Running %s' % ' '.join([str(x) for x in cmd_list]))
    process = subprocess.Popen(cmd_list, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = process.communicate()

    server.stop()

    log_multilines(logging.info, stdout)
    log_multilines(logging.info, stderr)

    if process.returncode != 0:
        raise TestFailureError('Test failed with exit code %s' % str(process.returncode), process.returncode)

    return stdout, stderr


def main():
    logging.basicConfig(level=logging.INFO)
    test_shield(sys.argv[1:])


if __name__ == "__main__":
    main()
