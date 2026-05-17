cmd_/root/spectre_research/modules.order := {   echo /root/spectre_research/ptewalk.ko; :; } | awk '!x[$$0]++' - > /root/spectre_research/modules.order
