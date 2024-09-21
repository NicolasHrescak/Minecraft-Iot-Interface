ws, err=http.websocket('ws://localhost:3000')

function display(y)
    print(y)
    assert(type(input) == 'number', 'input precisa ser um númreo')
    redstone.setOutput('top', y~=1 and y~=4)
    redstone.setOutput('left', y~=5 and y~=6)
    redstone.setOutput('right', y~=1 and y~=2 and y~=3 and y~=7)
    redstone.setOutput('bottom', y~=0 and y~=1 and y~=7)    

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
