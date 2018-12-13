#!/usr/bin/python

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

gdb.execute("catch syscall arch_prctl")
gdb.execute("break *main")

gdb.execute("r")
gdb.execute("c")
fs = gdb.parse_and_eval("$rsi")
gdb.execute("c")
rip = gdb.parse_and_eval("$rip")
rsp = gdb.parse_and_eval("$rsp")
pid = gdb.selected_inferior().pid

with open("/proc/{}/maps".format(pid), "r") as f:
    proc_map_entries = list(map(
        lambda x: ProcMapEntry(list(filter(lambda x: len(x) > 0, x.split(" ")))),
        filter(
            lambda x: len(x) > 0,
            map(lambda x: x.strip(), f.read().split("\n"))
        )
    ))

with open("/proc/{}/mem".format(pid), "rb") as memf, open(str(pid) + ".image", "wb") as outf:
    outf.write(struct.pack("<QQQI", rip, rsp, fs, len(proc_map_entries)))

    for entry in proc_map_entries:
        mem_dump = entry.dump_mem(memf)
        outf.write(struct.pack("<QI", entry.addr_begin, len(mem_dump)))
        for page in mem_dump:
            outf.write(page)
