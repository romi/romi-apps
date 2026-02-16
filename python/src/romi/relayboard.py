import time
from rcom.rcom_client import RcomWSClient
import argparse


class RelayBoard():

    @staticmethod
    def create(topic = 'relay', registry_ip = None):
        client = RcomWSClient(topic, topic, registry_ip)
        return RelayBoard(client)
   
    def __init__(self, client):
        self.client = client
        
    def count_relays(self):
        return self.client.execute('relay-board:count-relays')
        
    def get(self, index):
        return self.client.execute('relay-board:get', {'index': index})
        
    def set(self, index, value):
        return self.client.execute('relay-board:set', {'index': index, 'value': value})

        
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--topic', type=str, nargs='?', default="relay",
                        help='The registry topic')
    parser.add_argument('--registry', type=str, nargs='?', default=None,
                        help='The IP address of the registry')
    args = parser.parse_args()
    
    board = RelayBoard.create(args.topic, args.registry)
    count = board.count_relays()
    print(f"RelayBoard: {count} relays")
    for index in range(count):
        value = board.get(index)
        print(f"Relay {index}: {value}")
    
