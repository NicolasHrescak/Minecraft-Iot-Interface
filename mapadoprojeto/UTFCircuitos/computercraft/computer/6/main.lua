ws, err=http.websocket('ws://localhost:3000')

if ws then
    print('Conexão bem sucedida')
    ws.send('topic,foto')
    input=''
    while input~='-1' do
        input,err=ws.receive()
        if err then
            print(err)
        else
            if input=='definir,0' then
                redstone.setAnalogOutput('right', 0)
            else if input=='definir,1' then
                redstone.setAnalogOutput('right', 15)
            end
            end
        end
    end
else
    print(err)
end

