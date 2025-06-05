# cmos2bmp
A tool for converting CMOS raw images to bmp format.

#### Usage:
```
cmos2bmp -i input_file -o output_file -h hor_size -v ver_size [-t type] [-hb hor_blank]
    type:
        rgb565
        rgb888
        raw8
        raw8_rggb
        raw8_grbg
        raw8_bggr
        raw8_gbrg
        raw10
        raw10_rggb
        raw10_grbg
        raw10_bggr
        raw10_gbrg
        raw16
        yuv422_uyvy
        yuv422_yuyv
```
A example for raw8_rggb(8bit raw rggb format):
```
|<---hor_size-->|<--hor_blank-->|__
|R|G|R|G|...|R|G|...hor_blank...|v
|G|B|G|B|...|G|B|...hor_blank...|e
|R|G|R|G|...|R|G|...hor_blank...|r
|G|B|G|B|...|G|B|...hor_blank...|_
|R|G|R|G|...|R|G|...hor_blank...|s
|G|B|G|B|...|G|B|...hor_blank...|i
...                              z
|G|B|G|B|...|G|B|...hor_blank...|e
                                 __
```
#### Example:
convert 10bit rggb, 1920x1080, hor_blank=800 to bmp
```shell
cmos2bmp -i ./example.bin -o ./example.bmp -h 1920 -v 1080 -hb 800 -t raw10_rggb
```
