
# ![](https://raw.github.com/rbtnn/tile/master/images/icon_large.png) Tile

This is Tiling Window Manager for MS Windows.


## BUILD

> mingw32-make.exe -f makefile

* should define `BOOST_ROOT`.


## USAGE

> tile.exe ./tile.json


## LAYOUT METHODS

### arrange

![](https://raw.github.com/rbtnn/tile/master/images/arrange.png)


### arrange\_twin

![](https://raw.github.com/rbtnn/tile/master/images/arrange_twin.png)


### arrange\_maximal

![](https://raw.github.com/rbtnn/tile/master/images/arrange_maximal.png)


### arrange\_cross

![](https://raw.github.com/rbtnn/tile/master/images/arrange_cross.png)

### arrange\_square

...

### arrange\_manual

...

## CONFIGURATION

Configure file of `tile.exe` is `tile.json`.


### keys

Default key mappings are defined `keys` section.
You can edit it and customize key mappings.


#### focus\_window\_to\_master (`Alt + Ctrl + m`)

Focus an active window to master pane.

#### next\_layout (`Alt + Ctrl + Space`)

Change layout method.

#### next\_focus (`Alt + Ctrl + j`)

Focus to a next window.

#### previous\_focus (`Alt + Ctrl + k`)

Focus to a previous window.

#### exit\_tile (`Alt + Ctrl + q`)

Exit tile.exe.

#### kill\_client (`Alt + Ctrl + c`)

Kill an active window.

#### run\_process (`Alt + Ctrl + Enter`)

Run a process.

#### rescan (`Alt + Ctrl + r`)

Rescan processes.

#### toggle\_transparency\_window (`Alt + Ctrl + t`)

Toggle transparency to an active window.

#### swap\_next (`Alt + Ctrl + J`)

Swap an active window and a next window.

#### swap\_previous (`Alt + Ctrl + K`)

Swap an active window and a previous window.

#### workspace\_1 (`Alt + Ctrl + 1`)

Go to workspace 1.

#### workspace\_2 (`Alt + Ctrl + 2`)

Go to workspace 2.

#### workspace\_3 (`Alt + Ctrl + 3`)

Go to workspace 3.

#### workspace\_4 (`Alt + Ctrl + 4`)

Go to workspace 4.

#### workspace\_5 (`Alt + Ctrl + 5`)

Go to workspace 5.

#### workspace\_6 (`Alt + Ctrl + 6`)

Go to workspace 6.

#### workspace\_7 (`Alt + Ctrl + 7`)

Go to workspace 7.

#### workspace\_8 (`Alt + Ctrl + 8`)

Go to workspace 8.

#### workspace\_9 (`Alt + Ctrl + 9`)

Go to workspace 9.

#### move\_to\_workspace\_1 (`Alt + Ctrl + !`)

Move an active window to workspace 1.

#### move\_to\_workspace\_2 (`Alt + Ctrl + @`)

Move an active window to workspace 2.

#### move\_to\_workspace\_3 (`Alt + Ctrl + #`)

Move an active window to workspace 3.

#### move\_to\_workspace\_4 (`Alt + Ctrl + $`)

Move an active window to workspace 4.

#### move\_to\_workspace\_5 (`Alt + Ctrl + %`)

Move an active window to workspace 5.

#### move\_to\_workspace\_6 (`Alt + Ctrl + ^`)

Move an active window to workspace 6.

#### move\_to\_workspace\_7 (`Alt + Ctrl + &`)

Move an active window to workspace 7.

#### move\_to\_workspace\_8 (`Alt + Ctrl + *`)

Move an active window to workspace 8.

#### move\_to\_workspace\_9 (`Alt + Ctrl + (`)

Move an active window to workspace 9.


### settings

#### ignore\_classnames

...

#### not\_apply\_style\_to\_classnames

...

#### run\_process\_path

...

#### layout\_method\_names

...


## VIM INTERFACE

### Commands

#### :TileStart

...

#### :TileEditConfigureFile

...


## LICENSE

Distributed under MIT License. See LICENSE.txt


