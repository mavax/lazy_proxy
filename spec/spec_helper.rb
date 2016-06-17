require 'rubygems'
require 'rspec'
require 'pry'

$LOAD_PATH.unshift File.expand_path('lib')
require 'lazy_proxy'

RSpec.configure do |c|
  c.color = true
  c.tty = true
  c.formatter = :documentation
end
