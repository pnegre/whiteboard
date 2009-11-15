import installer

env = Environment()
env.Append(CCFLAGS = '-g')

opts = Options( 'options.conf', ARGUMENTS )
installer.AddOptions( opts )
opts.Update( env )
install = installer.Installer( env )

# We should check that gcc and g++ are installed
# HOW ??

# We should check that 'pkg-config' is installed also

deps = (
	'Xtst',
	'SDL',
	'cwiid',
	'SDL_gfx'
)

conf = Configure(env)
for l in deps:
	if not conf.CheckLib(l): exit(1)

env = conf.Finish()


libs = (
	'xtst',
	'sdl'
)

for l in libs:
	env.ParseConfig('pkg-config ' + l + ' --cflags --libs')


whiteboard = env.Program('whiteboard', Glob('*.c*'))
install.AddProgram(whiteboard)



