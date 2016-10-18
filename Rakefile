require 'rake/extensiontask'
require 'rspec/core/rake_task'

spec = Gem::Specification.load('lazy_proxy.gemspec')

task :benchmark do
  require 'lazy_proxy'
  require 'benchmark'

  benchmark = Benchmark.measure do
    1000000.times do
      LazyProxy.new(:object)
    end
  end.real
  puts benchmark
end

desc 'Run all specs'
RSpec::Core::RakeTask.new('test') do |s|
  s.rspec_opts = %w()
end

Rake::ExtensionTask.new('lazy_proxy', spec) do |ext|
  ext.lib_dir = 'ext/lazy_proxy'
end

task default: [:compile, :test]
