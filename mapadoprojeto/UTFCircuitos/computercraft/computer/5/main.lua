ws, err=http.websocket('ws://localhost:3000')

function display(y)
    print(y)
    assert(type(input) == 'number', 'input precisa ser um númreo')
    redstone.setOutput('left', y~=2)
    redstone.setOutput('bottom', y~=1 and y~=4 and y~=7)
    redstone.setOutput('right', y==0 or y==2 or y==6 or y==8)    

end


if ws then
    print('Conexão bem sucedida')
    ws.send('topic,display')
    input=''
    while input~='-1' do
        input,err=ws.receive()
        if err then
            print(err)
        else
            input = tonumber(input)
            display(input)
        end
    end
else
    print(err)
end
