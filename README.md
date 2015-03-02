# rlite-rb

[![Build Status](https://travis-ci.org/seppo0010/rlite-rb.svg?branch=master)](https://travis-ci.org/seppo0010/rlite-rb)

Ruby bindings for rlite. For more information about rlite, go to
[rlite repository](https://github.com/seppo0010/rlite)

## Installation

```bash
$ gem install hirlite
```

## Usage

### Using redis-rb

```ruby
require('redis')
require('hirlite/connection')
redis = Redis.new(:host => ":memory:", :driver => Rlite::Connection::Hirlite)
redis.set 'key', 'value'
p redis.get 'key' # value
```

### Without redis-rb

```ruby
require('hirlite')
r = Hirlite::Rlite.new()
r.connect(':memory:', 0)
r.write(['set', 'key', 'value'])
p r.read  # OK
r.write(['get', 'key'])
p r.read  # value
```

### Persistence

```ruby
1.9.3-p551 :001 > require('hirlite')
=> true
1.9.3-p551 :002 > r = Hirlite::Rlite.new()
=> #<Hirlite::Rlite:0x007f92508c4d50>
1.9.3-p551 :003 > r.connect('mydb.rld', 0)
=> nil
1.9.3-p551 :004 > r.write(['set', 'key', 'value'])
=> nil
1.9.3-p551 :005 > r.connect('mydb.rld', 0)
=> nil
1.9.3-p551 :006 > r.write(['get', 'key'])
=> nil
1.9.3-p551 :007 > r.read
=> "value"
```
