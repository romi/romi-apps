import time
from rcom.rcom_client import RcomWSClient
import argparse


class Fluo():

    @staticmethod
    def create(topic = 'fluo', registry_ip = None):
        client = RcomWSClient(topic, topic, registry_ip)
        return Fluo(client)
   
    def __init__(self, client):
        self.client = client
        
    def measure(self, intensity, length, frequency):
        params = {'intensity': intensity,
                  'length': length,
                  'frequency': frequency}
        return self.client.execute('fluo:measure', params)

        
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--topic', type=str, nargs='?', default="fluo",
                        help='The registry topic')
    parser.add_argument('--registry', type=str, nargs='?', default=None,
                        help='The IP address of the registry')
    args = parser.parse_args()
    
    fluo = Fluo.create(args.topic, args.registry)
    result = fluo.measure(0.5, 100, 2)
    measurements = result['measurements']
    print(measurements)
