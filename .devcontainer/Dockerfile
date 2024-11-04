FROM rocm/composable_kernel:ck_ub20.04_rocm6.0
# Add a new user
ARG USERNAME=vscode
ARG USER_UID=2092
ARG USER_GID=2005

# Create the user
RUN groupadd --gid $USER_GID $USERNAME \
    && groupadd --gid 109 rocm
RUN adduser --uid $USER_UID --gid $USER_GID --shell /bin/bash $USERNAME \
    && mkdir -p /home/$USERNAME \
    && chown -R $USERNAME:$USERNAME /home/$USERNAME \
    && usermod -aG sudo,video,rocm $USERNAME

# Enable sudo access
RUN echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers
    
ENV PATH="/opt/rocm/bin/:/opt/rocm/llvm/bin:${PATH}"