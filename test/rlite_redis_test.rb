gem 'minitest'
require 'minitest/autorun'

require 'redis'
require File.expand_path('../../lib/hirlite/connection', __FILE__)

module RliteRedisTests
  attr_reader :redis

  class ExtRliteTest < Minitest::Test
    def setup
      @redis = Redis.new(:host => ":memory:", :driver => Rlite::Connection::Hirlite)
    end

    def test_set_get
      @redis.set 'key', 'value'
      assert_equal "value", @redis.get('key')
    end
  end
end

