Gem::Specification.new do |s|
  s.name        = 'superstudio'
  s.version     = '0.9.0'
  s.date        = '2017-09-24'
  s.summary     = "An alternative way of thinking about creating JSON output: life without creating ruby objects."
  s.description = ""
  s.authors     = ["David Dawson"]
  s.email       = 'its.dave.dawson@gmail.com'
  s.files       = `git ls-files README.md LICENSE ext lib`.split
  s.homepage    = 'https://github.com/DaveTD/superstudio'
  s.license     = 'GPL-3.0'
  s.extensions  = %w[ext/superstudio/extconf.rb]
end
