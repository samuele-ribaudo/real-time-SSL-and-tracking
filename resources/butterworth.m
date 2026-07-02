Fs = 50000;
Fc1 = 300;   % High-pass cutoff
Fc2 = 3000;  % Low-pass cutoff
N = 6;       % Filter order

% Design the bandpass filter directly as Second-Order Sections (SOS)
[Z, P, K] = butter(N/2, [Fc1 Fc2]/(Fs/2), 'bandpass');
[sos, g] = zp2sos(Z, P, K);

% Combine the global gain 'g' into the first stage coefficients (b0, b1, b2)
sos(1, 1:3) = sos(1, 1:3) * g;

% Print coefficients formatted for C copy-pasting
disp('--- Cascaded Biquad Coefficients (SOS) ---');
for i = 1:size(sos,1)
    fprintf('.b0 = %10.8f, .b1 = %10.8f, .b2 = %10.8f, .a1 = %10.8f, .a2 = %10.8f\n', ...
            sos(i,1), sos(i,2), sos(i,3), sos(i,5), sos(i,6));
end