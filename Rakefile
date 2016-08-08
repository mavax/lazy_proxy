require 'rake/extensiontask'
require 'rspec/core/rake_task'

spec = Gem::Specification.load('lazy_proxy.gemspec')

desc 'Run all specs'
RSpec::Core::RakeTask.new('test') do |s|
  s.rspec_opts = %w()
end

Rake::ExtensionTask.new('lazy_proxy', spec) do |ext|
  ext.lib_dir = 'ext/lazy_proxy'
end

task default: [:compile, :test]
