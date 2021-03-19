import re
import villas.node.sample.villas_pb2
from datetime import datetime
from functools import total_ordering

@total_ordering
class Timestamp:
    """Parsing the VILLASnode human-readable timestamp format"""

    def __init__(self, seconds=None, nanoseconds=None,
                 offset=None, sequence=None):
        self.seconds = seconds
        self.nanoseconds = nanoseconds
        self.offset = offset
        self.sequence = sequence

    @classmethod
    def now(cls, offset=None, sequence=None):
        n = datetime.utcnow()

        secs = int(n.timestamp())
        nsecs = 1000 * n.microsecond

        return Timestamp(seconds=secs, nanoseconds=nsecs,
                         offset=offset, sequence=sequence)

    @classmethod
    def parse(cls, ts):
        m = re.match(r'(\d+)(?:\.(\d+))?([-+]\d+(?:\.\d+)?'
                     r'(?:e[+-]?\d+)?)?(?:\((\d+)\))?', ts)

        seconds = int(m.group(1))  # Mandatory
        nanoseconds = int(m.group(2)) if m.group(2) else None
        offset = float(m.group(3)) if m.group(3) else None
        sequence = int(m.group(4)) if m.group(4) else None

        return Timestamp(seconds, nanoseconds, offset, sequence)

    def __str__(self):
        str = "%u" % (self.seconds)

        if self.nanoseconds is not None:
            str += ".%09u" % self.nanoseconds
        if self.offset is not None:
            str += "+%u" % self.offset
        if self.sequence is not None:
            str += "(%u)" % self.sequence

        return str

    def __float__(self):
        sum = float(self.seconds)

        if self.nanoseconds is not None:
            sum += self.nanoseconds * 1e-9
        if self.offset is not None:
            sum += self.offset

        return sum

    def __eq__(self, other):
        return float(self) == float(other)

    def __lt__(self, other):
        return float(self) < float(other)


@total_ordering
class Sample:
    """Parsing a VILLASnode sample from a file (not a UDP package!!)"""

    def __init__(self, ts, values):
        self.ts = ts
        self.values = values

    @classmethod
    def parse(cls, line):
        return cls.decode(format='villas.human', line)

    @classmethod
    def decode(cls, format='protobuf', buffer):
        if format == 'protobuf':
            return cls.decode_protobuf(buffer)
        elif format == 'villas.human':
            return cls.decode_villas_human(buffer)
        else:
            raise NotImplementedError()

    def encode(self, format='protobuf'):
        if format == 'protobuf':
            return cls.encode_protobuf()
        elif format == 'villas.human':
            return cls.encode_villas_human()
        else:
            raise NotImplementedError()

    @classmethod
    def decode_villas_human(self, buf):
        csv = line.split()

        ts = Timestamp.parse(csv[0])
        vs = []

        for value in csv[1:]:
            try:
                v = float(value)
            except ValueError:
                value = value.lower()
                try:
                    v = complex(value)
                except Exception:
                    if value.endswith('i'):
                        v = complex(value.replace('i', 'j'))
                    else:
                        raise ValueError()

            vs.append(v)

        return Sample(ts, vs)

    def encode_villas_human(self):
        return bytes(self.__str__())

    @classmethod
    def decode_protobuf(cls, buffer):
        msg = villas_pb2.Message()
        msg.ParseFromString(buffer)

        samples = []
        for smp in msg:
            if smp.HasField('timestamp'):
                ts = Timestamp(
                    smp.timestamp.sec,
                    smp.timestamp.nsec
                )
            else:
                ts = Timestamp.now()

            if smp.HasField('sequence'):
                ts.sequence = smp.sequence

            values = []
            for val in smp.values:
                if val.HasField('f'):
                    values.append(val.f)
                elif val.HasField('i'):
                    values.append(val.i)
                elif val.HasField('b'):
                    values.append(val.b)
                elif val.HasField('z'):
                    values.append(val.z)

            sample = cls(ts, values)

    def encode_protobuf(self):
        msg = villas_pb2.Message()

        smp = msg.samples.add()

        ts = smp.timestamp.add()

        ts.sec = self.ts.seconds
        ts.nsec = self.ts.nanoseconds

        for value in self.values:
            val = smp.values.add()

            if type(value) is int:
                val.i = value
            elif type(value) is float:
                val.f = value
            elif type(value) is bool:
                val.b = value
            elif type(value) is complex:
                val.z.real = value.real
                val.z.imag = value.imag
        
        return msg.SerializeToString()

    def __str__(self):
        return '%s\t%s' % (self.ts, "\t".join(map(str, self.values)))

    def __eq__(self, other):
        return self.ts == other.ts

    def __lt__(self, other):
        return self.ts < other.ts
