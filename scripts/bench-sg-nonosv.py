from config import bench_out

import re

for data_set in bench_out.joinpath("bench-sg-nonosv").iterdir():
    masks = {}
    sizes = {}

    for fname in data_set.iterdir():
        mask_re = re.match(r"mask-0x(.+?)-.*", fname.name)
        size_re = re.match(r"size-0x(.+?)-.*", fname.name)
        key = ""
        dic = None
        if mask_re:
            key = mask_re.group(1)
            dic = masks

        elif size_re:
            key = size_re.group(1)
            dic = sizes

        with open(fname, "r") as f:
            text = f.read()
            nsecs = re.findall(r"([0-9]+) nanosecodes", text)

            if key not in dic:
                dic[key] = [0] * 6
            dic[key][0] += int(nsecs[0])
            dic[key][1] += int(nsecs[1])
            dic[key][2] += int(nsecs[2])
            dic[key][3] += int(nsecs[3])
            dic[key][4] += int(nsecs[4])
            dic[key][5] += int(nsecs[5])
    print("masks:")
    for key in sorted(masks.keys()):
        print("\t".join([
            key,
            str(masks[key][2] / 10),
            str(masks[key][3] / 10),
            str(masks[key][4] / 10),
            str(masks[key][5] / 10)
        ]))
    print("sizes:")
    for key in sorted(sizes.keys()):
        print("\t".join([
            key,
            str(sizes[key][2] / 10),
            str(sizes[key][3] / 10),
            str(sizes[key][4] / 10),
            str(sizes[key][5] / 10)
        ]))
