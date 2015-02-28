require "bundler"
Bundler::GemHelper.install_tasks

require "rbconfig"
require "rake/testtask"
require "rake/extensiontask"

unless defined?(RUBY_ENGINE) && RUBY_ENGINE == "jruby"

  Rake::ExtensionTask.new('hirlite_ext') do |task|
    # Pass --with-foo-config args to extconf.rb
    task.config_options = ARGV[1..-1] || []
    task.lib_dir = File.join(*['lib', 'hirlite', 'ext'])
  end

  namespace :hirlite do
    task :clean do
      # Fetch hirlite if not present
      if !File.directory?("vendor/rlite/.git")
        system("git submodule update --init")
      end
      RbConfig::CONFIG['configure_args'] =~ /with-make-prog\=(\w+)/
      make_program = $1 || ENV['make']
      unless make_program then
        make_program = (/mswin/ =~ RUBY_PLATFORM) ? 'nmake' : 'make'
      end
      system("cd vendor/rlite && #{make_program} clean")
    end
  end

  # "rake clean" should also clean bundled hirlite
  Rake::Task[:clean].enhance(['hirlite:clean'])

  # Build from scratch
  task :rebuild => [:clean, :compile]

else

  task :rebuild do
    # no-op
  end

end

task :default => [:rebuild, :test]

desc "Run tests"
Rake::TestTask.new(:test) do |t|
  t.pattern = 'test/**/*_test.rb'
  t.verbose = true
end
