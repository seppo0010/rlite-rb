require "hirlite/ext/hirlite_ext"

module Hirlite
  class Rlite < Hirlite::Ext::Rlite
    def initialize
      @responses = []
    end

    def write(command)
      super
      @responses.push read_ext
    end

    def read_ruby
      @responses.shift
    end

    alias_method :read_ext, :read
    alias_method :read, :read_ruby
  end
end
