ws, err=http.websocket('ws://localhost:3000')

if ws then
    print('Conexão bem sucedida')
    local estado=false
    local tgl=false;
    while true do
        local input=redstone.getInput('front')
        if input and not tgl then
            tgl=true
            estado=not estado
            ws.send(('send,lamp2,%d'):format(estado and 1 or 0))
            print(('lamp1 %s'):format(estado and 'acendeu' or 'apagou'))
        elseif not input then
            tgl=false
        end
        sleep(0.1)
    end
else
    print(err)
end
