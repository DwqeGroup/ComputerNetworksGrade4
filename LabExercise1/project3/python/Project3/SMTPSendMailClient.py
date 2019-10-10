from email.header import Header
from email.mime.text import MIMEText
from email.utils import parseaddr, formataddr

import smtplib


def _format_addr(s):
    name, addr = parseaddr(s)
    return formataddr((Header(name, 'utf-8').encode(), addr))


from_addr = 'zqydwqe@163.com'
password = 'zqyzqy123456'
# password = 'hbkisimwyrvkbfac'

to_addr = 'zqydwqe@sina.com'
smtp_server = 'smtp.163.com'

msg = MIMEText('很高心见到你，这是来自网络实验的测试邮件', 'plain', 'utf-8')
msg['From'] = _format_addr('zqydwqe<%s>' % from_addr)
msg['To'] = _format_addr('抱歉打扰 <%s>' % to_addr)
msg['Subject'] = Header('测试邮件', 'utf-8').encode()

server = smtplib.SMTP(smtp_server, 25)
server.set_debuglevel(1)
server.login(from_addr, password)
server.sendmail(from_addr, [to_addr], msg.as_string())
server.quit()
