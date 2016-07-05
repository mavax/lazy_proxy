require 'rake/extensiontask'
require 'rspec/core/rake_task'

spec = Gem::Specification.load('lazy_proxy.gemspec')

desc 'Check for memory leaks'
task :memory_leak_check do
  require 'lazy_proxy'
  top_level_threads = []
  5.times do
    top_level_threads << Thread.new do
      aux_class = Class.new
      threads = []
      5.times do
        threads << Thread.new do
          loop do
            1000.times { LazyProxy.new(aux_class.new) }
            GC.start
            print "#{ObjectSpace.each_object(aux_class).count}\n"
          end
        end
      end
      threads.each(&:join)
    end
  end
  top_level_threads.each(&:join)
end

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
