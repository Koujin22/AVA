import time
import zmq


def main():
    context = zmq.Context()

    # First, connect our subscriber socket
    subscriber = context.socket(zmq.SUB)
    subscriber.connect("tcp://127.0.0.1:5500")
    subscriber.setsockopt(zmq.SUBSCRIBE, b'TODO')

    time.sleep(1)

    # Second, synchronize with publisher
    syncclient = context.socket(zmq.REQ)
    syncclient.connect("tcp://127.0.0.1:5501")

    # send a synchronization request
    print("Sending syn")
    syncclient.send(b'syn_TODO')

    # wait for synchronization reply
    msg = syncclient.recv()
    print("Recieved: ", msg)

    # Third, get our updates and report how many we got
    
    msg = subscriber.recv()
    print("Recieved MSG from sub: ", msg)

    syncclient.send(b'say_What do you want me to write down?')



def Printstuff():
    print("Aqui ando todo tranqui.")

if __name__ == "__main__":
    main()