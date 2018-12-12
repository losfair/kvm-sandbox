import sys

REG_NAMES = ["rdi", "rsi", "rdx", "r10", "r8", "r9"]

with open(sys.argv[1], "r") as f:
    code = list(filter(
        lambda x: len(x) > 0,
        map(lambda x: x.strip(), f.read().split("\n")),
    ))

out = ""

for i in range(len(code)):
    try:
        line = code[i]
        [name, args] = line.split("(", 1)
        if args[-1] != ')':
            raise Exception("expecting ')'")

        out += "case " + name + ": {\n"

        args = list(filter(
            lambda x: len(x) > 0,
            map(lambda x: x.strip(), args[:-1].split(","))
        ))

        for j in range(len(args)):
            arg = args[j]
            if arg[0] == '@':
                out += "uint64_t translated_{} = translate_address(vcpu_fd, regs.{});\n".format(j, REG_NAMES[j])
                [len_idx, unit_len] = list(map(lambda x: x.strip(), arg[1:].split("#")))

                unit_len = int(unit_len)
                if len_idx == "_":
                    mem_bound = "1"
                else:
                    mem_bound = "regs." + REG_NAMES[int(len_idx)]

                out += "check_guest_mem_bounds(translated_{}, {} * {});\n".format(
                    j,
                    mem_bound,
                    unit_len,
                )

        out += "regs.rax = syscall({},".format(name)
        for j in range(len(args)):
            if args[j][0] == '@':
                out += "&guest_mem[translated_{}],".format(j)
            else:
                out += "regs." + REG_NAMES[j] + ","
        out = out[:-1]
        out += ");\n"
        out += "break;\n}\n\n"
    except Exception as e:
        print("failed to parse line " + str(i + 1))
        raise e

print(out)