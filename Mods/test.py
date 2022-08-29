import time
import zmq
from threading import Thread


def client(context):
    socket = context.socket(zmq.REQ)
    socket.connect("tcp://localhost:5559")
    for request in range(1, 11):
        socket.send(b"test")
        msg = socket.recv()
        print("Got msg: "+msg.decode())


def worker(context):
    socket = context.socket(zmq.REP)
    socket.connect("tcp://localhost:5560")
    
    for request in range(1, 11):
        msg = socket.recv()
        print(f"Got req: {msg}")
        socket.send(b"response")

def broker(context):
    router = context.socket(zmq.ROUTER)
    dealer = context.socket(zmq.DEALER)
    router.bind("tcp://*:5559")
    dealer.bind("tcp://*:5560")

    poller = zmq.Poller()
    poller.register(router, zmq.POLLIN)
    poller.register(dealer, zmq.POLLIN)

    for request in range(1, 21):
        socks = dict(poller.poll())


        if socks.get(router) == zmq.POLLIN:
            message = router.recv_multipart()
            print(message)
            dealer.send_multipart(message)

        if socks.get(dealer) == zmq.POLLIN:
            message = dealer.recv_multipart()
            print(message)
            router.send_multipart(message)


def main():
    context = zmq.Context()
    t_client = Thread(target=client, args=(context,))
    t_worker = Thread(target=worker, args=(context,))
    t_broker = Thread(target=broker, args=(context,))
    t_broker.start()
    t_worker.start()
    t_client.start()

    t_client.join()
    print("Client join")
    t_worker.join()
    print("worker join")
    t_broker.join()
    print("broker join")


if __name__ == "__main__":
    main()