local ws
local err
local input

ws,err=http.websocket('ws://localhost:3000')
if err then
    print(err)
else
    print('Conexão bem sucedida!')
    ws.send('topic,chuva')
    input=''
    while input~='-1' do
        input,err=ws.receive()
        if err then
            print(err)
        else
            input=tonumber(input)
            if input~=nil and input>0 then
                commands.weather('rain', ('%ds'):format(input))
            else
                commands.weather('rain')
            end
        end
    end
end