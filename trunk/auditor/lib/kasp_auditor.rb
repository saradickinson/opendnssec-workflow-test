#
# $Id$
#
# Copyright (c) 2009 Nominet UK. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
# IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
# IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

require 'syslog'
include Syslog::Constants
require 'lib/kasp_auditor/config.rb'
require 'lib/kasp_auditor/auditor.rb'
require 'lib/kasp_auditor/parse.rb'

module KASPAuditor
  class Runner
    def run(path, filename="zonelist.xml")
      #      path = ARGV[0] + "/"
      Syslog.open("kasp_auditor") { |syslog| run_with_syslog(path, filename, syslog)
      }
    end
    def run_with_syslog(path, filename, syslog)
      zones = Parse.parse(path, filename, syslog)
      # Now check the input and output zones using the config
      auditor = Auditor.new(syslog)
      zones.each {|config, input_file, output_file|
        auditor.check_zone(config, input_file, output_file)
      }
      ret = 0 # @TODO@ Return value to controlling process
      exit(ret)
    end
  end
end
