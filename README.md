# rlite-py

[![Build Status](https://travis-ci.org/seppo0010/rlite-rb.svg?branch=master)](https://travis-ci.org/seppo0010/rlite-rb)

Ruby bindings for rlite. For more information about rlite, go to
[rlite repository](https://github.com/seppo0010/rlite)

## Installation

```bash
$ gem install hirlite
```

## Usage

```ruby
1.9.3-p551 :001 > require('hirlite')
=> true
1.9.3-p551 :002 > r = Hirlite::Ext::Rlite.new()
=> #<Hirlite::Ext::Rlite:0x007f90230ae730>
1.9.3-p551 :003 > r.connect(':memory:', 0)
=> nil
1.9.3-p551 :004 > r.write(['set', 'key', 'value'])
=> nil
1.9.3-p551 :005 > r.read
=> true
1.9.3-p551 :006 > r.write(['get', 'key'])
=> nil
1.9.3-p551 :007 > r.read
=> "value"
```

### Persistence

```python
1.9.3-p551 :001 > require('hirlite')
=> true
1.9.3-p551 :002 > r = Hirlite::Ext::Rlite.new()
=> #<Hirlite::Ext::Rlite:0x007f92508c4d50>
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
