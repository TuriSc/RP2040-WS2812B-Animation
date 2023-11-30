#!/usr/bin/env python3

import sys
import os
from PIL import Image

# Convert images to uRGB32_t arrays for use with
# RP2040-WS2812B-Animation library.

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print ("Usage: %s [image]" % (sys.argv[0],))
        sys.exit(1)

    image = Image.open(sys.argv[1])
    bitmap = image.load()

    filename = os.path.splitext(sys.argv[1])[0]
    print ("Converting file: %s" % filename)
    f = open(filename + ".h", "w+")
    f.write("static const uGRB32_t %s[]={\n" % (filename.upper()))

    for y in range(0, image.size[1]):
        for x in range(0, image.size[0]):
            r, g, b, a = bitmap[x, y]
            grb = "0x00"
            grb += "{:02x}".format(g)
            grb += "{:02x}".format(r)
            grb += "{:02x}".format(b)
            f.write("%s, " % grb)
        f.write("\n")

    f.write("};\n")
    f.close()