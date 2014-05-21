#!/usr/bin/env ruby

require 'ffi'

module LibDisasm
  extend FFI::Library
  ffi_lib '/opt/owasp-syd/multitool/libdis/libdisasm.so'
  attach_function :calculate_something, [:int, :float], :double
  attach_function :error_code, [], :int # note empty array for functions taking zero arguments
  attach_function :create_object, [:string], :pointer
  attach_function :calculate_something_else, [:double, :pointer], :double
  attach_function :free_object, [:pointer], :void
end
