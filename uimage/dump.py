import subprocess
import argparse
import struct

IGNORED_SECTIONS = set(["[vvar]", "[vsyscall]"])

class ProcMapEntry:
    def __init__(self, columns):
        [self.addr_begin, self.addr_end] = list(map(lambda x: int(x, 16), columns[0].split("-")))
        self.name = columns[5]

    def dump_mem(self, memf):
        if self.name in IGNORED_SECTIONS:
            return []

        ret = []
        memf.seek(self.addr_begin)
        pos = self.addr_begin
        while pos < self.addr_end:
            page = memf.read(4096)
            ret.append(page)
            pos += 4096
        return ret


parser = argparse.ArgumentParser()
parser.add_argument("--pid", type = int, help = "pid of process to dump", required = True)
parser.add_argument("--rip", type = lambda x: int(x, 16), help = "initial value of rip", required = True)
parser.add_argument("--rsp", type = lambda x: int(x, 16), help = "initial value of rsp", required = True)
parser.add_argument("--fs", type = lambda x: int(x, 16), help = "initial value of fs", required = True)
parser.add_argument("-o", "--out", type = str, help = "file to write dump", required = True)

args = parser.parse_args()

with open("/proc/{}/maps".format(args.pid), "r") as f:
    proc_map_entries = list(map(
        lambda x: ProcMapEntry(list(filter(lambda x: len(x) > 0, x.split(" ")))),
        filter(
            lambda x: len(x) > 0,
            map(lambda x: x.strip(), f.read().split("\n"))
        )
    ))


with open("/proc/{}/mem".format(args.pid), "rb") as memf, open(args.out, "wb") as outf:
    outf.write(struct.pack("<QQQI", args.rip, args.rsp, args.fs, len(proc_map_entries)))

    for entry in proc_map_entries:
        mem_dump = entry.dump_mem(memf)
        outf.write(struct.pack("<QI", entry.addr_begin, len(mem_dump)))
        for page in mem_dump:
            outf.write(page)
