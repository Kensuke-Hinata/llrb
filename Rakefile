require "bundler/gem_tasks"
require "rspec/core/rake_task"

RSpec::Core::RakeTask.new(:spec)

require "rake/extensiontask"

task :build => :compile

Rake::ExtensionTask.new("llruby") do |ext|
  ext.lib_dir = "lib/llruby"
end

task :default => [:compile, :spec]
