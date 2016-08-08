[![Build Status](https://travis-ci.org/mavax/lazy_proxy.svg?branch=master)](https://travis-ci.org/mavax/lazy_proxy)

## Efficient lazy proxy for Ruby

Can wrap any object or block returning an object.

```ruby
LazyProxy.new(:object)
LazyProxy.new { :object }
```
Block is only evaluated on first use.
