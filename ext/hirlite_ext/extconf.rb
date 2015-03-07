require 'mkmf'

RbConfig::MAKEFILE_CONFIG['CC'] = ENV['CC'] if ENV['CC']

hirlite_dir = File.join(File.dirname(__FILE__), %w{.. .. vendor rlite})
unless File.directory?(hirlite_dir)
  STDERR.puts "vendor/rlite missing, please checkout its submodule..."
  exit 1
end

RbConfig::CONFIG['configure_args'] =~ /with-make-prog\=(\w+)/
make_program = $1 || ENV['make']
make_program ||= case RUBY_PLATFORM
when /mswin/
  'nmake'
when /(bsd|solaris)/
  'gmake'
else
  'make'
end

# Make sure hirlite is built...
Dir.chdir(hirlite_dir) do
  success = system("#{make_program} libhirlite.a lua")
  raise "Building hirlite failed" if !success
end

# Statically link to hirlite (mkmf can't do this for us)
$CFLAGS << " -I#{hirlite_dir}/src"
$LDFLAGS << " #{hirlite_dir}/src/libhirlite.a #{hirlite_dir}/deps/lua/src/liblua.a"

have_func("rb_thread_fd_select")
create_makefile('hirlite/ext/hirlite_ext')
