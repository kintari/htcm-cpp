
# revealed testing
.\build\Debug\htcm-cpp.exe .\data\20230605-195519 | jq 'select(.buff.name=="Hide in Shadows" and .buff.remove==1)|{ when: .time, who: .agent.name }'
