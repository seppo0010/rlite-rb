require "hirlite/ext/hirlite_ext"

module Hirlite
  class Rlite < Hirlite::Ext::Rlite
    def initialize
      @responses = []
    end

    def write(command)
      super
      @responses.push self.class.superclass.instance_method(:read).bind(self).call
    end

    def read
      @responses.shift
    end
  end
end
