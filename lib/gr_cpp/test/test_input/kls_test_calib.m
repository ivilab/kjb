% This is sooooo hacked together --Kyle

colormap(gray(256));

while(true)

cur_pt = 1;
P = ones(5, 8 * 8 * 3);

for i = 0 : 8
    for j = 0:8
        for k = 1:3
            switch k
                case 1
                    P_cur = [i j 8]';
                case 2
                    P_cur = [0 i j]';
                case 3 
                    P_cur = [j 0 i]';
            end
            P(:, cur_pt) = [k; 20 * P_cur; 1];
            cur_pt = cur_pt + 1;
        end
    end
end


i = 1;
for  M = M_list
    p = M{1} * P(2:5, :);
    p = bsxfun(@rdivide, p, p(3,:));
    eval(['I = I_' num2str(i) ';']);
    image(I);
    hold on;
    plot(p(1,:), p(2,:), 'rx');
    title(['Image ' num2str(i)]);
    hold off;
    pause

    i = i + 1;

    if(mod(i + 1, 9) == 0)
        switch uint8((i + 2) / 9)
            case 1
                ind = find(P(1,:) == 3);
                P(3,ind) = 160;
            case 2
                ind = find(P(1,:) == 2);
                P(2,ind) = 160;
            case 3
                ind = find(P(1,:) == 3);
                P(3,ind) = 0;
            end
    end
end
end
