require File.expand_path("../lib/hirlite/version", __FILE__)

Gem::Specification.new do |s|
  s.name = "hirlite"
  s.version = Hirlite::VERSION
  s.homepage = "http://github.com/seppo0010/rlite-rb"
  s.authors = ["Sebastian Waisbrot"]
  s.email = ["seppo0010@gmail.com"]
  s.license = 'BSD-3-Clause'
  s.summary = "Ruby wrapper for hirlite"
  s.description = s.summary

  s.require_path = "lib"
  s.files = []

  if RUBY_PLATFORM =~ /java/
    s.platform = "java"
  else
    s.extensions = Dir["ext/**/extconf.rb"]
    s.files += Dir["ext/**/*.{rb,c,h}"]
    s.files += Dir["vendor/rlite/src/*.{c,h}"] +
      Dir["vendor/rlite/deps/lua/Makefile"] +
      Dir["vendor/rlite/deps/lua/src/*.{c,h}"] +
      Dir["vendor/rlite/Makefile"] +
      Dir["vendor/rlite/src/Makefile"]
  end

  s.files += Dir["lib/**/*.rb"]
  s.files += %w(LICENSE Rakefile)

  s.add_development_dependency "rake", "10.0"
  s.add_development_dependency "rake-compiler", "~> 0.7.1"
  s.add_development_dependency "minitest", "~> 5.5.1"
end
