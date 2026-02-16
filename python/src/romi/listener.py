import time
from rcom.rcom_client import RcomWSClient
import argparse


class Listener():

    @staticmethod
    def create(topic, registry_ip = None):
        client = RcomWSClient(topic, topic, registry_ip)
        return Listener(client)
   
    def __init__(self, client):
        self.client = client
        
    def recv(self):
        return self.client._recv()

        
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--topic', type=str, required=True,
                        help='The registry topic')
    parser.add_argument('--registry', type=str, nargs='?', default=None,
                        help='The IP address of the registry')
    args = parser.parse_args()
    
    listener = Listener.create(args.topic, args.registry)
    while True:
        message = listener.recv()
        print(f"{message}")
