require "redis/connection/registry"
require "redis/errors"
require "hirlite"
require "timeout"

class Rlite
  module Connection
    class Hirlite

      def self.connect(config)
        connection = ::Hirlite::Rlite.new
        connect_timeout = (config.fetch(:connect_timeout, 0) * 1_000_000).to_i

        if config[:scheme] == "unix"
          connection.connect_unix(config[:path], connect_timeout)
        else
          connection.connect(config[:host], config[:port], connect_timeout)
        end

        instance = new(connection)
        instance.timeout = config[:timeout]
        instance
      rescue Errno::ETIMEDOUT
        raise TimeoutError
      end

      def initialize(connection)
        @connection = connection
      end

      def connected?
        @connection && @connection.connected?
      end

      def timeout=(timeout)
        # Hirlite works with microsecond timeouts
        @connection.timeout = Integer(timeout * 1_000_000)
      end

      def disconnect
        @connection.disconnect
        @connection = nil
      end

      def write(command)
        @connection.write(command.flatten(1))
      rescue Errno::EAGAIN
        raise TimeoutError
      end

      def read
        reply = @connection.read
        if reply == nil
            write(['__rlite_poll'])
            reply = @connection.read
        end
        reply = CommandError.new(reply.message) if reply.is_a?(RuntimeError)
        reply
      rescue Errno::EAGAIN
        raise TimeoutError
      rescue RuntimeError => err
        raise ProtocolError.new(err.message)
      end
    end
  end
end

Redis::Connection.drivers << Rlite::Connection::Hirlite
