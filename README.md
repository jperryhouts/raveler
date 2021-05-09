Raveler
======

Convert any image to a continuous path of overlapping line segments.  
Try it at [https://jperryhouts.github.io/raveler/](https://jperryhouts.github.io/raveler/)

![demo](web/demo.gif)

## Usage

Raveler has two interfaces: a web interface and a command-line interface. More options are available in the CLI version, including the ability to change the frame size, number of pins, thread weight, thread color, etc. However, the web interface is much more convenient. The web interface is live at [https://jperryhouts.github.io/raveler/](https://jperryhouts.github.io/raveler/) and requires no configuration on your part. The command-line interface must be compiled locally.

### Compiling the CLI app

On Debian/Ubuntu linux that can be accomplished as follows:

Install build dependencies:
```bash
sudo apt install build-essential graphicsmagick-libmagick-dev-compat
```

Compile the CLI application:
```bash
git clone https://github.com/jperryhouts/raveler.git
cd raveler
make cli
sudo cp build/cli/raveler /usr/bin
```

And run it:
```bash
raveler -f svg -o raveled.svg web/volcano.jpg
```
The above command will generate a rendering of your string art in the file '`raveled.svg`'.

You can find other options using the help flag:
```bash
raveler --help
```

## Credits

This project was inspired by Petros Vrellis ["A New Way to Knit" (2016)](http://artof01.com/vrellis/works/knit.html).

Default image: Karymsky volcano (Kamchatka), 2004. Alexander Belousov  
(CC Attribution-NonCommercial-ShareAlike via [imaggeo.egu.eu](https://imaggeo.egu.eu/view/646/)).


## License

Copyright (C) 2021 Jonathan Perry-Houts

This program is free software: you can redistribute it and/or modify  
it under the terms of the GNU General Public License as published by  
the Free Software Foundation, either version 3 of the License, or  
(at your option) any later version.

This program is distributed in the hope that it will be useful,  
but WITHOUT ANY WARRANTY; without even the implied warranty of  
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the  
GNU General Public License for more details.

The full text of the license can be found in the [LICENSE](LICENSE) file  
of this repository.
