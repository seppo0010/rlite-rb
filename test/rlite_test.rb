gem 'minitest'
require 'minitest/autorun'

require File.expand_path('../../lib/hirlite/rlite', __FILE__)

module RliteTests
  attr_reader :hirlite

  class ExtRliteTest < Minitest::Test
    def setup
      @hirlite = Hirlite::Rlite.new
      @hirlite.connect ":memory:", 0
    end

    def test_none
      @hirlite.write(['get', 'key'])
      assert_nil @hirlite.read
    end

    def test_true
      @hirlite.write(['set', 'key', 'value'])
      assert_equal true, @hirlite.read
    end

    def test_string
      @hirlite.write(['set', 'key', 'value'])
      @hirlite.read
      @hirlite.write(['get', 'key'])
      assert_equal 'value', @hirlite.read
    end

    def test_integer
      @hirlite.write(['lpush', 'list', 'value', 'other value'])
      assert_equal 2, @hirlite.read
    end

    def test_error
      @hirlite.write(['set', 'key'])
      assert_instance_of RuntimeError, @hirlite.read
    end

    def test_array
      @hirlite.write(['rpush', 'mylist', '1', '2', '3'])
      @hirlite.read
      @hirlite.write(['lrange', 'mylist', '0', '-1'])
      assert_equal ['1', '2', '3'], @hirlite.read
    end

    def test_buffered_reads
      @hirlite.write(['set', 'key', 'value'])
      @hirlite.write(['get', 'key'])
      @hirlite.read
      assert_equal 'value', @hirlite.read
    end
  end

  class ExtRlitePersitentTest < Minitest::Test
    PATH = 'rlite.rld'
    def connect
      @hirlite = Hirlite::Rlite.new
      @hirlite.connect PATH, 0
    end

    def setup
      FileUtils.rm(PATH) if File.file?(PATH)
      connect
    end

    def teardown
      FileUtils.rm(PATH) if File.file?(PATH)
    end

    def test_write_close_open
      @hirlite.write(['set', 'key', 'value'])
      @hirlite.read
      connect  # close db, open a new one
      @hirlite.write(['get', 'key'])
      assert_equal 'value', @hirlite.read
    end
  end
end
