fs = 50000; % 50 kHz Sampling rate

% 1. High-Pass filter coefficients (300 Hz cutoff)
[b_hp, a_hp] = butter(2, 300 / (fs/2), 'high');
fprintf('--- HPF Coefficients (300 Hz) ---\n');
fprintf('b0 = %.8f, b1 = %.8f, b2 = %.8f\n', b_hp(1), b_hp(2), b_hp(3));
fprintf('a1 = %.8f, a2 = %.8f\n\n', a_hp(2), a_hp(3));

% 2. Low-pass filter coefficients (3000 Hz cutoff)
[b_lp, a_lp] = butter(2, 3000 / (fs/2), 'low');
fprintf('--- LPF Coefficients (3000 Hz) ---\n');
fprintf('b0 = %.8f, b1 = %.8f, b2 = %.8f\n', b_lp(1), b_lp(2), b_lp(3));
fprintf('a1 = %.8f, a2 = %.8f\n', a_lp(2), a_lp(3));
