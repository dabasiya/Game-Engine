#Build

for build you have to install cmake

#windows:

cmake ./





Ctrl + E to Open and Close Editor mode of Engine

in Editor Mode Ctrl + C for open console

#console commands

#1.load scene

load scene_name.scene

#2.clear command

for clear command line

#3.reset command

for reset scene



# for add new script to engine

to add new script to engine you have to derive entityscript component it has three virtual function, void start(), void update(float deltatime), void onEvent(Event& e).

after create script component you have to register it in scriptmanager component with a unique name.
