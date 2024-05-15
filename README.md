# Meteor

A simple breakout like game with a light level editor. Join a wonderful 
adventure in space.

### Requirements

You need to install **sdl2,sdl2-image** and **sdl2-font** for this game :

With Arch Linux :
```bash
pacman -Syu sdl2 sdl2_image sdl2_ttf
```

With Debian/Ubuntu :
```bash
apt-get install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev
```

### Build

Simply run the makefile :
```bash
make
```

### Run

Then you can build and run the game :
```bash
make run
```

or simply run the binary :
```bash
./meteor
```

### Game Controls

**Menu** :

- **Up/Down** : Select an option

**In game** :

- **Left/Right** : Move the spaceship (*You can also use the mouse*)
- **Space** : Launch a new ball
- **Escape** : Pause the game

**In editor** :

- **Left Click** : Place or move a brick
- **Right Click** : Remove a brick
