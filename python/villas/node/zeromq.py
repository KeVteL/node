import zmq
import zmq.asyncio

class Interface:

    def __init__(self, format):
        self.format = format


class ZeroMQInterface(Interface):

    def __init__(self, format):
        super().__init__(format)

        self.context = zmq.Context()
        self.socket = context.socket(zmq.REP)

    @property
    def config(self):
        return {
            'type': 'zeromq',
            'pattern': 'pubsub',
            'ipv6': False,

            'curve': { # Z85 encoded Curve25519 keys
                'enabled': False,
                'public_key': 'Veg+Q.V-c&1k>yVh663gQ^7fL($y47gybE-nZP1L'
                'secret_key': 'HPY.+mFuB[jGs@(zZr6$IZ1H1dZ7Ji*j>oi@O?Pc'
            }

            'in': {
                'subscribe': '',
                'filter': self.filter
            },
            'out': {
                'publish': [
                    ''
                ],
                'filter': self.filter
            }
        }

    def connect(self, addr):
        self.socket.connect(addr)

    def bind(self, addr):
        self.socket.bind(addr)

    def send(self, sample):
        buffer = sample.encode(self.format, sample)

        self.socket.send(buffer)

    def recv(self):
        buffer = self.socket.recv()

        return Sample.decode(self.format, buffer)


class AsyncZeroMQNode(ZeroMQNode):

    def __init__(self, format):
        Interface.__init__(self, format)

        self.context = zmq.asyncio.Context()
        self.socket = context.socket(zmq.REP)

    async def send(self, sample):
        buffer = sample.encode(self.format, sample)

        await self.socket.send(buffer)

    async def recv(self):
        buffer = await self.socket.recv()

        return Sample.decode(self.format, buffer)
