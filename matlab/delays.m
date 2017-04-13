function result = delays(filename)
    [pcm, samplerate] = audioread(filename);
    t1 = pcm(:,1);
    t2 = pcm(:,2);
    len = length(t1);
    cnt = 1;
    clicks = floor(len / samplerate);
    result = zeros(1, clicks);
    currentClick = 1;
    disp(clicks);
    while cnt + samplerate < len
        disp([currentClick, clicks]);
        delay = finddelay(t1(cnt:cnt+samplerate), t2(cnt:cnt+samplerate));
        result(currentClick) = delay;
        cnt = cnt + samplerate;
        currentClick = currentClick + 1;
    end
end