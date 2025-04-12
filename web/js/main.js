// Add scroll event listener to handle header style changes and logo switch
window.addEventListener('scroll', () => {
    const header = document.querySelector('.header');
    const logo = document.getElementById('header-logo');
    const heroSection = document.querySelector('.hero');
    
    if (heroSection) {
        const heroHeight = heroSection.offsetHeight;
        const scrollTrigger = heroHeight * 0.8; // 设置触发点为hero section高度的80%
        
        if (window.scrollY > scrollTrigger) {
            header.classList.add('scrolled');
            // 当页面滚动超过hero section 80%时切换为深色logo
            logo.src = './images/company-logo.png';
        } else {
            header.classList.remove('scrolled');
            // 当回到触发点以上时切换为浅色logo
            logo.src = './images/company-logo-white.png';
        }
    } else {
        // 如果没有hero section（例如在登录页面），保持scrolled状态
        header.classList.add('scrolled');
    }
});

// Toggle password visibility
document.addEventListener('DOMContentLoaded', function() {
    const togglePassword = document.getElementById('togglePassword');
    const password = document.getElementById('password');
    
    if(togglePassword && password) {
        togglePassword.addEventListener('click', function() {
            // Toggle the type attribute
            const type = password.getAttribute('type') === 'password' ? 'text' : 'password';
            password.setAttribute('type', type);
            
            // Toggle the icon
            this.querySelector('i').classList.toggle('fa-eye');
            this.querySelector('i').classList.toggle('fa-eye-slash');
        });
    }
    
    // Add animation to login form
    const loginForm = document.getElementById('loginForm');
    if(loginForm) {
        const formGroups = loginForm.querySelectorAll('.form-group');
        formGroups.forEach((group, index) => {
            group.style.opacity = '0';
            group.style.transform = 'translateY(20px)';
            setTimeout(() => {
                group.style.transition = 'all 0.5s ease';
                group.style.opacity = '1';
                group.style.transform = 'translateY(0)';
            }, 100 * (index + 1));
        });
    }
});

// Photo slider controls and animation effects
document.addEventListener('DOMContentLoaded', () => {
    // Add animation effect for technical showcase cards
    const techCards = document.querySelectorAll('.tech-card');
    
    // Intersection Observer to trigger animations when elements come into view
    const observer = new IntersectionObserver((entries) => {
        entries.forEach(entry => {
            if (entry.isIntersecting) {
                entry.target.classList.add('show');
                observer.unobserve(entry.target);
            }
        });
    }, { threshold: 0.2 });
    
    // Observe each tech card
    techCards.forEach(card => {
        observer.observe(card);
    });
    
    const sliderDots = document.querySelectorAll('.slider-dot');
    const photoSlider = document.querySelector('.photo-slider');
    let autoSlideInterval;
    let currentSlideIndex = 0;
    const animationDuration = 15000; // 15秒，与CSS动画持续时间相匹配
    
    // 更新轮播点的激活状态
    function updateActiveDot(index) {
        document.querySelector('.slider-dot.active').classList.remove('active');
        sliderDots[index].classList.add('active');
        currentSlideIndex = index;
    }
    
    // 自动轮播控制函数，可以指定从哪个幻灯片开始
    function startAutoSlide(startFromIndex = 0) {
        // 清除之前的任何自动轮播计时器
        if (autoSlideInterval) {
            clearInterval(autoSlideInterval);
        }
        
        // 设置当前幻灯片索引
        currentSlideIndex = startFromIndex;
        
        // 立即更新导航点状态
        updateActiveDot(currentSlideIndex);
        
        // 移除所有自定义样式，恢复自动动画，但从当前幻灯片开始
        photoSlider.classList.remove('manual-control');
        photoSlider.style.animation = 'none';
        photoSlider.offsetHeight; // 触发重绘
        
        // 设置初始位置为当前幻灯片
        photoSlider.style.transform = `translateX(-${currentSlideIndex * 33.333}%)`;
        
        // 创建自定义动画，从当前幻灯片开始
        let customAnimation = `
            @keyframes sliderCustom {
                0%, 25% {
                    transform: translateX(-${currentSlideIndex * 33.333}%);
                }
                33%, 58% {
                    transform: translateX(-${((currentSlideIndex + 1) % 3) * 33.333}%);
                }
                66%, 91% {
                    transform: translateX(-${((currentSlideIndex + 2) % 3) * 33.333}%);
                }
                100% {
                    transform: translateX(-${currentSlideIndex * 33.333}%);
                }
            }
        `;
        
        // 添加自定义动画样式
        let styleElement = document.getElementById('custom-slider-style');
        if (!styleElement) {
            styleElement = document.createElement('style');
            styleElement.id = 'custom-slider-style';
            document.head.appendChild(styleElement);
        }
        styleElement.textContent = customAnimation;
        
        // 应用自定义动画
        photoSlider.style.animation = 'sliderCustom 15s infinite ease-in-out';
        
        // 设置动画监听器来更新导航点
        photoSlider.addEventListener('animationiteration', () => {
            // 动画完成一次迭代时重置计时器
            clearInterval(autoSlideInterval);
            startAutoSlide(currentSlideIndex);
        }, { once: true });
        
        // 设置定时器更新导航点
        autoSlideInterval = setInterval(() => {
            currentSlideIndex = (currentSlideIndex + 1) % 3;
            updateActiveDot(currentSlideIndex);
        }, animationDuration / 3);
    }
    
    // 初始启动自动轮播
    startAutoSlide();
    
    // 为导航点添加点击事件
    sliderDots.forEach((dot) => {
        dot.addEventListener('click', () => {
            // 获取data-slide属性值并转换为数字
            const slideIndex = parseInt(dot.getAttribute('data-slide'));
            
            // 更新激活的导航点
            updateActiveDot(slideIndex);
            
            // 停止当前的自动轮播
            clearInterval(autoSlideInterval);
            
            // 移动轮播到对应的幻灯片
            photoSlider.style.animation = 'none';
            photoSlider.offsetHeight; // 触发重绘，确保动画停止
            photoSlider.style.transform = `translateX(-${slideIndex * 33.333}%)`;
            photoSlider.classList.add('manual-control');
            
            // 设置定时器，5秒后恢复自动轮播，从当前选中的幻灯片开始
            setTimeout(() => {
                startAutoSlide(slideIndex);
            }, 5000);
        });
    });
});