import sys

def_file = sys.argv[1]
export_names_file = sys.argv[2]
exports_file = sys.argv[3]

names=[]
for line in open(def_file):
    line=line.strip()
    if not line:
        continue
    if line.startswith(";"):
        continue
    if line.startswith("LIBRARY"):
        continue
    if line.startswith("EXPORTS"):
        continue

    parts=line.split()
    name=parts[0].strip()
    names.append(name)

with open(export_names_file,"w") as f:
    for n in names:
        if n.startswith("ord_"):
            f.write("nullptr,\n")
        else:
            f.write('"%s",\n' % n)
    f.write("nullptr\n")

with open(exports_file,"w") as f:
    f.write("""
.intel_syntax noprefix
.text
.extern proc_table

""")

    for i,n in enumerate(names):
        f.write(f"""
.global {n}
.def {n}; .scl 2; .type 32; .endef

{n}:
    jmp QWORD PTR [rip + proc_table + {i*8}]

""")
