gem 'minitest'
require 'minitest/autorun'

require 'redis'
require File.expand_path('../../lib/hirlite/connection', __FILE__)

module RliteRedisTests
  attr_reader :redis

  class ExtRliteTest < Minitest::Test
    def new_redis
      Redis.new(:host => ":memory:", :driver => Rlite::Connection::Hirlite)
    end

    def setup
      @redis = new_redis
    end

    def test_set_get
      @redis.set 'key', 'value'
      assert_equal "value", @redis.get('key')
    end
  end

  class ExtRlitePersitentTest < Minitest::Test
    PATH = 'rlite.rld'
    def new_redis
      Redis.new(:host => PATH, :driver => Rlite::Connection::Hirlite)
    end

    def connect
      @redis = new_redis
    end

    def setup
      FileUtils.rm(PATH) if File.file?(PATH)
      connect
    end

    def teardown
      @redis.client.disconnect
      @redis = nil
      FileUtils.rm(PATH) if File.file?(PATH)
    end

    def test_subscribe_and_unsubscribe
      @subscribed = false
      @unsubscribed = false

      wire = Thread.new do
        @redis.subscribe("foo") do |on|
          on.subscribe do |channel, total|
            @subscribed = true
            @t1 = total
          end

          on.message do |channel, message|
            if message == "s1"
              @redis.unsubscribe
              @message = message
            end
          end

          on.unsubscribe do |channel, total|
            @unsubscribed = true
            @t2 = total
          end
        end
      end

      # Wait until the subscription is active before publishing
      Thread.pass while !@subscribed

      r = new_redis
      r.publish("foo", "s1")
      r.client.disconnect
      r = nil

      wire.join

      assert @subscribed
      assert_equal 1, @t1
      assert @unsubscribed
      assert_equal 0, @t2
      assert_equal "s1", @message
    end
  end
end
