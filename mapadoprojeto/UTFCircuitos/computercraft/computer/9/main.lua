ws, err=http.websocket('ws://localhost:3000')

if ws then
    print('Conexão bem sucedida')
    local tgl=false
    local input = ''
    while true do
        input=redstone.getAnalogInput('left')
        if input>0 and not tgl then
            print(input-5)
            tgl=true
            ws.send(string.format('send,display,%d', input-5))
        elseif input==0 then
            tgl=false
        end
        sleep(0.1)
    end
else
    print(err)
end
