// // 检测用户登录状态的函数
// function checkLoginStatus() {
//     fetch('/check_login', {
//         method: 'GET',
//         headers: {
//             'Content-Type': 'application/json',
//             // 获取login返还的sessionID
//             'X-Session': sessionStorage.getItem("session_id") // 替换为实际的会话ID
//         }
//     })
//     .then(response => response.json())
//     .then(data => {
//         if (data.success === true) {
//             // 用户已登录，执行当前页面的逻辑，例如显示内容或跳转页面
//             console.log('用户已登录');
//         } else {
//             // 用户未登录，强制跳转到登录页面或执行其他逻辑
//             console.log('用户未登录，需要登录');
//             // 示例：强制跳转到登录页面
//             window.location.href = '/login.html'; // 根据实际路径修改
//         }
//     })
//     .catch(error => {
//         console.error('检测登录状态出错:', error);
//     });
// }

// // 在页面加载完成后调用检测登录状态的函数
// document.addEventListener('DOMContentLoaded', function() {
//     checkLoginStatus();
// });
