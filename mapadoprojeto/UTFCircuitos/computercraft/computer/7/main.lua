ws, err=http.websocket('ws://localhost:3000')

if ws then
    print('Conexão bem sucedida')
    local estado=false
    
    while true do
        local input=redstone.getAnalogInput('right')
        if input <6 and not estado then
            estado=not estado
            ws.send('send,luz,1')
            print(('lamp1 %s'):format(estado and 'acendeu' or 'apagou'))
        else if input>=6 and estado then
            ws.send('send,luz,0')
            estado = not estado
        end
        end
        sleep(0.1)
    end
else
    print(err)
end
