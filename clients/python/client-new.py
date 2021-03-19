import math

from villas.node.node import Node
from villas.node.zeromq import ZeroMQNode


def main():
    zmq = ZeroMQInterface()

    zmq_config = zmq.config
    zmq_config['in']['signals'] = [
        {
            'name': 'a',
            'type': 'float'
        },
        {
            'name': 'b',
            'type': 'float'
        }
    ]

    hook_signals = [
        {
            'name': 'sum',
            'type': 'float',
            'expression': 'smp.data.a + smp.data.b'
        },
        {
            'name': 'hyp',
            'type': 'float',
            'expression': 'math.sqrt(smp.data.a^2 + smp.data.b^2)'
        },
        {
            'name': 'max',
            'type': 'float',
            'expression': 'math.max(smp.data.sine, smp.data[3])'
        }
    ]

    config = {
        'nodes': {
            'zmq': zmq_config,
            'sin_a': {
                'type': 'signal'
                'signal': 'sine',
                'phase': 0
            },
            'sin_b': {
                'type': 'signal'
                'signal': 'sine',
                'phase': math.pi / 2
            }
        },
        'paths': [
            {
                'in': [
                    'zmq.a',
                    'zmq.b',
                    'sin_a.sine',
                    'sin_b.sine'
                ],
                'out': 'zmq',
                'hooks': hook_signals
            }
        ]
    }

    node = Node(config)
    node.start()

    while x in range(0, 100):
        smp = 

        zmq.send(smp)

        smp_recv = zmq.recv()

        # assert smp_recv

    node.stop()


if __name__ == '__main__':
    main()
