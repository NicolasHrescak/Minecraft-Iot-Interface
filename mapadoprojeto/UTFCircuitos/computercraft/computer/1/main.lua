ws, err=http.websocket('ws://localhost:3000')

if ws then
    print('Conexão bem sucedida')
    ws.send('topic,lamp1')
    input=''
    while input~='-1' do
        input,err=ws.receive()
        if err then
            print(err)
        else
            if input~='0' then
                redstone.setAnalogOutput('right', 0)
            else
                redstone.setAnalogOutput('right', 15)
            end
        end
    end
else
    print(err)
end

