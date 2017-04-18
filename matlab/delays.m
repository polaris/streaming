function result = delays(filename)
    [pcm, fs] = audioread(filename);
    t1 = pcm(:,1); t2 = pcm(:,2);
    len = length(t1);
    clicks = floor(len / fs);
    result = zeros(1, clicks);
    sample = 1; count = 1;
    while sample + fs < len
        result(count) = finddelay(t1(sample:sample+fs), t2(sample:sample+fs));
        sample = sample + fs;
        count = count + 1;
    end
end
