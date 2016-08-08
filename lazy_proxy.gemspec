lib = File.expand_path('../lib', __FILE__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)

Gem::Specification.new do |s|
  s.name          = 'lazy_proxy'
  s.version       = '0.0.1'
  s.authors       = ['Manuel A. Valenzuela']
  s.email         = ['manvalaco@gmail.com']

  s.summary       = <<-EOF
  Efficient lazy object proxy
  EOF
  s.description   = s.summary

  s.files         = `git ls-files -z`.split("\x0").reject do |f|
    f.match(%r{^(test|spec|features)/})
  end
  s.require_paths = %w(lib ext)

  s.extensions << 'ext/lazy_proxy/extconf.rb'
  s.add_development_dependency 'rake-compiler'
  s.add_development_dependency 'rake'
  s.required_ruby_version = '>= 2.1.0'
end
